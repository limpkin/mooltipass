package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Qoqa extends AbstractPage{

	public Qoqa(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	
	@FindBy(id = "menu-panel-trigger")
	private WebElement menuBtn;
	
	@FindBy(xpath = "//div[text()='Se connecter']")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//input[@id='user-login']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='user-password']")
	private WebElement password;

	@FindBy(xpath = "//button[@id='session-sign-in-button']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[text()='Se déconnecter']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//button[text()='Ça marche !']")
	private WebElement popup;
	
	public void goToLogin(){
		if(waitUntilAppears(popup))
			popup.click();
		menuBtn.click();
		waitUntilAppears(loginBtn);
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		}
	
	public boolean checkLogin(){

		
		return isElementPresent(By.xpath("//span[contains(text(),'Hello, citesting')]"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@id='user-login']"));
	}
	
	public void logout(){
		menuBtn.click();
		waitUntilAppears(logoutBtn);
		logoutBtn.click();

	}
}