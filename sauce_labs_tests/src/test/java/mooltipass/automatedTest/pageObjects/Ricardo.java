package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Ricardo extends AbstractPage{

	public Ricardo(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "it_OverlayLogin")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@id='Login']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='Password']")
	private WebElement password;

	@FindBy(xpath = "//button[@id='modalsubmit']")
	private WebElement submitLogin;
	
	@FindBy(id = "hd-logout")
	private WebElement logoutBtn;
	
	
	public void goToLogin(){
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

		waitUntilAppears(By.id("hd-logout"));
		return isElementPresent(By.id("hd-logout"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@id='Login']"));
	}
	
	public void logout(){
		logoutBtn.click();

	}
}