package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Metacritic extends AbstractPage{

	public Metacritic(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	
	@FindBy(xpath = "//a[@id='login']")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@id='login_email']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='login_password']")
	private WebElement password;

	@FindBy(xpath = "//button[@id='login']")
	private WebElement submitLogin;
	
	@FindBy(id = "login")
	private WebElement logoutBtn;
	
	
	public void goToLogin(){
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

		waitUntilAppears(By.id("primary_menu_user_profile"));
		return isElementPresent(By.id("primary_menu_user_profile"));
	}


	public boolean checkAtLoginPage(){
		return isElementPresent(By.xpath("//input[@id='login_email']"));
	}
	
	public void logout(){
		logoutBtn.click();

	}
}
