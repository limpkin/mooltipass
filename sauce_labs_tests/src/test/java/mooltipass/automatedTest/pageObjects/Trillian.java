package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Trillian extends AbstractPage{

	public Trillian(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(xpath = "//input[@id='x_loginUsername']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='x_loginPassword']")
	private WebElement password;

	@FindBy(xpath = "//div[text()='Sign in']")
	private WebElement submitLogin;
	
	@FindBy(id = "x_identityName")
	private WebElement user;
	
	@FindBy(id = "x_menuSignOut")
	private WebElement logoutBtn;
		
	@FindBy(id = "signOutConfirm")
	private WebElement logOutConfirm;
	
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		}
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//*[contains(text(),', mooltipass!')]"));
		return isElementPresent(By.xpath("//*[contains(text(),', mooltipass!')]"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@id='x_loginUsername']"));
	}
	
	public void logout(){
		user.click();
		logoutBtn.click();
		logOutConfirm.click();
	}
		
}
