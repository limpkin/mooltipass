package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Tindie extends AbstractPage{

	public Tindie(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(xpath = "//li/a[@href='#signin-register-modal']")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@id='id_auth-username']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='id_auth-password']")
	private WebElement password;

	@FindBy(xpath = "//input[@id='submit-id-login']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//ul[@class='dropdown-menu fade']/li/a[text()='Logout']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//li[@class='active-account dropdown']")
	private WebElement user;
	
	
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

		//waitUntilAppears(By.xpath("//li[@class='active-account dropdown']"));
		return isElementPresent(By.xpath("//li[@class='active-account dropdown']"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@id='id_auth-username']"));
	}
	
	public void logout(){
		user.click();
		logoutBtn.click();

	}

}
